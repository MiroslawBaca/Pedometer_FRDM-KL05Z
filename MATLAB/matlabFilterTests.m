function matlabFilterTests()

    %% ======================== UART & Global Variables =========================
    % Configure the serial port (adjust 'COM6' and baud rate as needed)
    serialPort = serialport('COM6', 9600);

    % Global lines for step detection in subplots 2 and 3
    global hLine5 hLine6
    global hLine7 hLine8 newStepCount lastStepIndex secondMethodThreshold

    % Global lines for HPF subplot (per-axis filtering)
    global hLineHPF hLineHPFStep hLineHPFRun hLineHPFMax

    % Global lines for BPF subplot (per-axis filtering)
    global hLineBPF hLineBPFStep hLineBPFRun hLineBPFMax

    % Global arrays to store raw data and magnitudes
    global accData accMagnitude

    %% ==================== Filter Parameter Definitions =======================
    % Set the sampling rate to 10 Hz
    samplingRate = 10;

    % High-pass filter (HPF) cutoff frequency
    hpfCutoffFreq = 2;
    [bHPF, aHPF] = butter(2, hpfCutoffFreq / (samplingRate / 2), 'high');

    % Bandpass filter (BPF) cutoff frequencies (0.3–2 Hz)
    bpfLowCutoffFreq = 0.3;
    bpfHighCutoffFreq = 2;
    [bBPF, aBPF] = butter(2, [bpfLowCutoffFreq/(samplingRate/2), bpfHighCutoffFreq/(samplingRate/2)], 'bandpass');
    % Test [bBPF, aBPF] = butter(10, [bpfLowCutoffFreq/(samplingRate/2), bpfHighCutoffFreq/(samplingRate/2)], 'bandpass');

    %% ===== Persistent States for Per-Axis Filtering (Subplots 4 & 5) =====
    persistent hpfStateX hpfStateY hpfStateZ
    if isempty(hpfStateX)
        hpfStateX = zeros(max(length(aHPF), length(bHPF)) - 1, 1);
        hpfStateY = zeros(max(length(aHPF), length(bHPF)) - 1, 1);
        hpfStateZ = zeros(max(length(aHPF), length(bHPF)) - 1, 1);
    end

    persistent bpfStateX bpfStateY bpfStateZ
    if isempty(bpfStateX)
        bpfStateX = zeros(max(length(aBPF), length(bBPF)) - 1, 1);
        bpfStateY = zeros(max(length(aBPF), length(bBPF)) - 1, 1);
        bpfStateZ = zeros(max(length(aBPF), length(bBPF)) - 1, 1);
    end

    %% ===== Persistent States for Magnitude-Based Filtering (Subplot 6) =====
    persistent magHPFState
    if isempty(magHPFState)
        magHPFState = zeros(max(length(aHPF), length(bHPF)) - 1, 1);
    end

    %% ==================== Subplot & Figure Definitions =======================
    figure;

    % --- Subplot 1: X, Y, Z axes ---
    subplot(6,1,1);
    hLine1 = animatedline('Color','r');  
    hLine2 = animatedline('Color','g');
    hLine3 = animatedline('Color','b');
    xlabel('Sample index');
    ylabel('Acceleration');
    legend('X axis', 'Y axis', 'Z axis');

    % --- Subplot 2: Raw Magnitude + Simple Step Detection ---
    subplot(6,1,2);
    hLine4 = animatedline('Color','magenta'); 
    hLine5 = animatedline('Color','black','Marker','o','LineStyle','none'); 
    hLine6 = animatedline('Color','red','Marker','o','LineStyle','none');
    xlabel('Sample index');
    ylabel('Magnitude');
    title('Simple Step Detection (Raw Magnitude)');
    legend('Raw Magnitude', 'Walk Marker', 'Run Marker');

    % --- Subplot 3: Raw Magnitude + Advanced Step Detection (Local Maxima) ---
    subplot(6,1,3);
    hLine8 = animatedline('Color','magenta'); 
    hLine7 = animatedline('Color','blue','Marker','o','LineStyle','none');
    xlabel('Sample index');
    ylabel('Magnitude');
    title('Advanced Detection - Raw Magnitude');
    legend('Raw Magnitude', 'Step Marker (Local Maxima)');

    % --- Subplot 4: HPF Magnitude (Per-Axis Filtering) ---
    subplot(6,1,4);
    hLineHPF = animatedline('Color','cyan'); 
    hLineHPFStep = animatedline('Color','black','Marker','o','LineStyle','none');
    hLineHPFRun = animatedline('Color','red','Marker','o','LineStyle','none');
    hLineHPFMax = animatedline('Color','blue','Marker','o','LineStyle','none');
    xlabel('Sample index');
    ylabel('Magnitude');
    title('HPF - Filtered Magnitude (Per Axis)');
    legend('HPF Signal', 'HPF Walk Marker', 'HPF Run Marker', 'HPF Local Maxima');

    % --- Subplot 5: BPF Magnitude (Per-Axis Filtering) ---
    subplot(6,1,5);
    hLineBPF = animatedline('Color','magenta');  
    hLineBPFStep = animatedline('Color','black','Marker','o','LineStyle','none');
    hLineBPFRun = animatedline('Color','red','Marker','o','LineStyle','none');
    hLineBPFMax = animatedline('Color','green','Marker','o','LineStyle','none');
    xlabel('Sample index');
    ylabel('Magnitude');
    title('BPF - Bandpass Filtered Magnitude (Per Axis)');
    legend('BPF Signal', 'BPF Walk Marker', 'BPF Run Marker', 'BPF Local Maxima');

    % --- Subplot 6: Combined HPF & BPF on Raw Magnitude ---
    subplot(6,1,6);
    global hLine6HPFMag hLine6BPFMag hLine6HPFRunMag hLine6BPFRunMag
    hLine6HPFMag = animatedline('Color','cyan');      % HPF on magnitude
    hLine6BPFMag = animatedline('Color','magenta');    % BPF on magnitude
    hLine6HPFRunMag = animatedline('Color','blue','Marker','s','LineStyle','none');   % HPF run marker (blue)
    hLine6BPFRunMag = animatedline('Color','green','Marker','s','LineStyle','none');   % BPF run marker (green)
    xlabel('Sample index');
    ylabel('Magnitude');
    title('HPF & BPF + Markers');
    legend('HPF Magnitude', 'BPF Magnitude', 'HPF Run Marker', 'BPF Run Marker');

    %% ==================== Global Variables Initialization ====================
    % Initialize global arrays using preallocated buffers
    persistent accDataBuffer accMagBuffer bufferCapacity
    if isempty(accDataBuffer)
        bufferCapacity = 1000; % initial capacity
        accDataBuffer = zeros(bufferCapacity, 3);
        accMagBuffer = zeros(bufferCapacity, 1);
    end
    accData = [];
    accMagnitude = [];

    oldAccValue = 0;
    countRunSteps = 0;
    countWalkSteps = 0;

    newStepCount = 0;
    lastStepIndex = 0;

    % Thresholds for the simple (difference-based) method
    oldMethodStepThreshold = 0.2;
    oldMethodRunThreshold = 0.7;

    % Threshold for the local maxima method on raw magnitude
    secondMethodThreshold = 1.05;

    %% === Persistent Variables for Old Method (HPF/BPF Difference) ===
    persistent oldHPFValue oldBPFValue
    if isempty(oldHPFValue)
        oldHPFValue = 0;
        oldBPFValue = 0;
    end

    %% === Buffers for Local Maxima on HPF/BPF (Per Axis) ===
    persistent lastHPFpeakIndex lastBPFpeakIndex
    if isempty(lastHPFpeakIndex)
        lastHPFpeakIndex = 0;
        lastBPFpeakIndex = 0;
    end

    % Thresholds and minimal sample distance for local maxima (per axis)
    hpfPeakThreshold = 0.3;
    bpfPeakThreshold = 0.3;
    minHPFSampleDist = 3;  % at Fs=10 -> 0.3 s
    minBPFSampleDist = 7;

    % Threshold for run/walk decision on Subplot 6
    runWalkThreshold6 = 1.3;

    %% === Persistent window buffers for local maxima detection ===
    % Fixed-size buffers (last 3 values) to avoid dynamic array growth
    persistent rawMagWin hpfWin bpfWin hpfMagWin bpfMagWin
    if isempty(rawMagWin)
        rawMagWin = nan(3,1);
    end
    if isempty(hpfWin)
        hpfWin = nan(3,1);
    end
    if isempty(bpfWin)
        bpfWin = nan(3,1);
    end
    if isempty(hpfMagWin)
        hpfMagWin = nan(3,1);
    end
    if isempty(bpfMagWin)
        bpfMagWin = nan(3,1);
    end

    %% ======================== Main Reading Loop =============================
    try
        sampleIndex = 0;

        while true

            %% =============== 1) Read from Serial Port ======================
            dataLine = readline(serialPort);
            data = str2double(split(dataLine, "  "));
            if numel(data) < 3
                % Skip if fewer than 3 values
                continue;
            end

            %% =============== 2) Update Sample Index ========================
            sampleIndex = sampleIndex + 1;

            %% =============== 3) Save Raw Data and Compute Magnitude =========
            % Preallocate buffer
            if sampleIndex > bufferCapacity
                newCapacity = bufferCapacity * 2;
                accDataBuffer(newCapacity, :) = 0;
                accMagBuffer(newCapacity, 1) = 0;
                bufferCapacity = newCapacity;
            end
            accDataBuffer(sampleIndex, :) = data';
            currentMagnitude = sqrt(data(1)^2 + data(2)^2 + data(3)^2);
            accMagBuffer(sampleIndex) = currentMagnitude;
            % Update global arrays (full history maintained)
            accData = accDataBuffer(1:sampleIndex, :);
            accMagnitude = accMagBuffer(1:sampleIndex);

            %% === Update raw magnitude window for new step detection ===
            if sampleIndex == 1
                rawMagWin(1) = currentMagnitude;
            elseif sampleIndex == 2
                rawMagWin(2) = currentMagnitude;
            else
                rawMagWin = [rawMagWin(2); rawMagWin(3); currentMagnitude];
            end

            %% =============== 4) Simple Step Detection (Raw Magnitude) =========
            if (oldAccValue - currentMagnitude >= oldMethodRunThreshold)
                countRunSteps = countRunSteps + 1;
                addpoints(hLine6, sampleIndex, currentMagnitude);  % Run marker
            elseif (oldAccValue - currentMagnitude >= oldMethodStepThreshold)
                countWalkSteps = countWalkSteps + 1;
                addpoints(hLine5, sampleIndex, currentMagnitude);  % Walk marker
            end
            oldAccValue = currentMagnitude;

            %% =============== 5) Plot Raw Axes (Subplot 1) =====================
            addpoints(hLine1, sampleIndex, data(1));
            addpoints(hLine2, sampleIndex, data(2));
            addpoints(hLine3, sampleIndex, data(3));

            %% =============== 6) Plot Raw Magnitude (Subplot 2) ==============
            addpoints(hLine4, sampleIndex, currentMagnitude);

            %% =============== 7) Plot Raw Magnitude (Subplot 3) ==============
            addpoints(hLine8, sampleIndex, currentMagnitude);

            %% =============== 8) Advanced Step Detection - Local Maxima (Raw) =========
            if sampleIndex >= 3
                if (rawMagWin(2) > rawMagWin(1)) && (rawMagWin(2) > rawMagWin(3)) && ...
                   (rawMagWin(2) > secondMethodThreshold) && ((sampleIndex - 1) - lastStepIndex > 3)
                    newStepCount = newStepCount + 1;
                    addpoints(hLine7, sampleIndex - 1, rawMagWin(2));
                    lastStepIndex = sampleIndex - 1;
                end
            end

            %% =============== 9) HPF Filtering (Per-Axis) =======================
            [xHPF, hpfStateX] = filter(bHPF, aHPF, data(1), hpfStateX);
            [yHPF, hpfStateY] = filter(bHPF, aHPF, data(2), hpfStateY);
            [zHPF, hpfStateZ] = filter(bHPF, aHPF, data(3), hpfStateZ);
            magHPF = sqrt(xHPF^2 + yHPF^2 + zHPF^2);
            addpoints(hLineHPF, sampleIndex, magHPF);

            % Simple difference-based detection on HPF
            if (oldHPFValue - magHPF >= oldMethodRunThreshold)
                addpoints(hLineHPFRun, sampleIndex, magHPF);   % Run marker
            elseif (oldHPFValue - magHPF >= oldMethodStepThreshold)
                addpoints(hLineHPFStep, sampleIndex, magHPF);    % Walk marker
            end
            oldHPFValue = magHPF;

            %% === Update HPF window buffer ===
            if sampleIndex == 1
                hpfWin(1) = magHPF;
            elseif sampleIndex == 2
                hpfWin(2) = magHPF;
            else
                hpfWin = [hpfWin(2); hpfWin(3); magHPF];
            end

            % Local maxima detection on HPF (Per-Axis)
            if sampleIndex >= 3
                if (hpfWin(2) > hpfWin(1)) && (hpfWin(2) > hpfWin(3)) && ...
                   (hpfWin(2) > hpfPeakThreshold) && ((sampleIndex - 1) - lastHPFpeakIndex > minHPFSampleDist)
                    addpoints(hLineHPFMax, sampleIndex - 1, hpfWin(2));
                    
                    % For Subplot 6: use HPF marker if magnitude exceeds threshold
                    if magHPF > runWalkThreshold6
                        addpoints(hLine6HPFRunMag, sampleIndex - 1, hpfWin(2));
                    end
                    lastHPFpeakIndex = sampleIndex - 1;
                end
            end

            %% =============== 10) BPF Filtering (Per-Axis) ======================
            [xBPF, bpfStateX] = filter(bBPF, aBPF, data(1), bpfStateX);
            [yBPF, bpfStateY] = filter(bBPF, aBPF, data(2), bpfStateY);
            [zBPF, bpfStateZ] = filter(bBPF, aBPF, data(3), bpfStateZ);
            magBPF = sqrt(xBPF^2 + yBPF^2 + zBPF^2);
            addpoints(hLineBPF, sampleIndex, magBPF);

            % Simple difference-based detection on BPF
            if (oldBPFValue - magBPF >= oldMethodRunThreshold)
                addpoints(hLineBPFRun, sampleIndex, magBPF);   % Run marker
            elseif (oldBPFValue - magBPF >= oldMethodStepThreshold)
                addpoints(hLineBPFStep, sampleIndex, magBPF);    % Walk marker
            end
            oldBPFValue = magBPF;

            %% === Update BPF window buffer ===
            if sampleIndex == 1
                bpfWin(1) = magBPF;
            elseif sampleIndex == 2
                bpfWin(2) = magBPF;
            else
                bpfWin = [bpfWin(2); bpfWin(3); magBPF];
            end

            % Local maxima detection on BPF (Per-Axis)
            if sampleIndex >= 3
                if (bpfWin(2) > bpfWin(1)) && (bpfWin(2) > bpfWin(3)) && ...
                   (bpfWin(2) > bpfPeakThreshold) && ((sampleIndex - 1) - lastBPFpeakIndex > minBPFSampleDist)
                    addpoints(hLineBPFMax, sampleIndex - 1, bpfWin(2));
                    
                    % For Subplot 6: use BPF marker if HPF magnitude does not exceed threshold
                    if magHPF <= runWalkThreshold6
                        addpoints(hLine6BPFRunMag, sampleIndex - 1, bpfWin(2));
                    end
                    lastBPFpeakIndex = sampleIndex - 1;
                end
            end

            %% =============== 11) Merge Magnitude from HPF and BPF =====
            hpfMagVal = magHPF;
            bpfMagVal = magBPF;
            
            addpoints(hLine6HPFMag, sampleIndex, hpfMagVal);
            addpoints(hLine6BPFMag, sampleIndex, bpfMagVal);
            
            %% === Update HPF and BPF magnitude window buffers for Subplot 6 ===
            if sampleIndex == 1
                hpfMagWin(1) = hpfMagVal;
                bpfMagWin(1) = bpfMagVal;
            elseif sampleIndex == 2
                hpfMagWin(2) = hpfMagVal;
                bpfMagWin(2) = bpfMagVal;
            else
                hpfMagWin = [hpfMagWin(2); hpfMagWin(3); hpfMagVal];
                bpfMagWin = [bpfMagWin(2); bpfMagWin(3); bpfMagVal];
            end

            %% =============== 12) Update the Plot ===========================
            drawnow limitrate;

        end

    catch
        % Close the serial port and figure in case of interruption
        closeSerialPort(gcf, serialPort);
    end

    % Final close after exiting the loop
    closeSerialPort(gcf, serialPort);

end

%% ====================== Close Serial Port Function =======================
function closeSerialPort(fig, serialPort)
    if isvalid(serialPort)
        delete(serialPort);
        disp('Serial port closed.');
    end
    if isvalid(fig)
        delete(fig);
    end
end
