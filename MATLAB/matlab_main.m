%closeSerialPort(gcf, serialPort);
% Konfiguracja portu szeregowego
serialPort = serialport('COM3', 9600); % Ustawienie prędkości transmisji


% Inicjalizacja wykresu
figure;
subplot(3,1,1);
hLine1 = animatedline('Color', 'r');
hLine2 = animatedline('Color', 'g');
hLine3 = animatedline('Color', 'b');
xlabel('Czas');
ylabel('Wartość');
legend('Oś x', 'Oś y', 'Oś z');

subplot(3,1,2);
hLine4 = animatedline('Color', 'magenta');
global hLine5;
global hLine6;
hLine5 = animatedline('Color', 'black',"Marker","o", "LineStyle","none");
hLine6 = animatedline('Color', 'red',"Marker","o", "LineStyle","none");

% ---------- ALGORYTM v2: Animated line dla pików (wykrywanie kroków) -------------
global hLine7;
hLine7 = animatedline('Color', 'blue',"Marker","o", "LineStyle","none");
xlabel('Czas');
ylabel('Magnitude');

global ZBIOR;
ZBIOR = [0 0 0; 0 0 0; 0 0 0];

global MAGNITUDE;
MAGNITUDE = [1; 1; 1];

liczba_krokow_bieg = 0;
liczba_krokow = 0;

accelerationVectorOld = 1;

% Odczyt i wyświetlanie danych na bieżąco
try
    while (1)
        % Odczyt całej linii z portu szeregowego
        dataLine = readline(serialPort);
        
        % Podział linii na poszczególne wartości
        data = str2double(split(dataLine, "  "));
        ZBIOR = [ZBIOR; rot90(data)];

        accelerationVector = sqrt(data(1)^2 + data(2)^2 + data(3)^2);
        MAGNITUDE = [MAGNITUDE; accelerationVector];

        if (accelerationVectorOld - accelerationVector >= 0.7)
            liczba_krokow_bieg = liczba_krokow_bieg + 1;
            addpoints(hLine6, length(MAGNITUDE), accelerationVector);
        elseif (accelerationVectorOld - accelerationVector >= 0.2)
            liczba_krokow = liczba_krokow + 1;
            addpoints(hLine5, length(MAGNITUDE), accelerationVector);
        end
        
        accelerationVectorOld = accelerationVector;

        % Aktualizacja danych na wykresie
        clearpoints(hLine1);
        clearpoints(hLine2);
        clearpoints(hLine3);
        clearpoints(hLine4);

        addpoints(hLine1, 1:length(ZBIOR), ZBIOR(:, 1));
        addpoints(hLine2, 1:length(ZBIOR), ZBIOR(:, 2));
        addpoints(hLine3, 1:length(ZBIOR), ZBIOR(:, 3));
        addpoints(hLine4, 1:length(ZBIOR), MAGNITUDE);
        
        % =========== ALGORYTM v2 START ==========
        % Wykrywanie kroków metodą detekcji pików w ostatnim oknie próbek
        try
            windowSizeV2 = 10;  % liczba próbek do analizy
            if length(MAGNITUDE) >= windowSizeV2
                dataWindow = MAGNITUDE(end-windowSizeV2+1:end);
                % DEBUG: wypisanie rozmiaru MAGNITUDE i zawartości okna
                fprintf('Długość MAGNITUDE = %d\n', length(MAGNITUDE));
                fprintf('dataWindow: '); disp(dataWindow');
                
                minPeakHeight = 1.05;  % próg wykrywania pików – do dostosowania
                minPeakDistance = 1;   % minimalna odległość między pikami (w próbkach)
                %[pks, locs] = findpeaks(dataWindow, 'MinPeakHeight', minPeakHeight, 'MinPeakDistance', minPeakDistance);
                [pks, locs] = find_local_peaks(dataWindow, minPeakHeight, minPeakDistance);

                % Przekształcamy indeksy pików do globalnej skali (dla całego wektora MAGNITUDE)
                globalLocs = (length(MAGNITUDE) - windowSizeV2) + locs;
                % Czyszczenie i rysowanie wykrytych pików na animatedline hLine7
                clearpoints(hLine7);
                for i = 1:length(pks)
                    addpoints(hLine7, globalLocs(i), pks(i));
                end
            end
        catch algErr
            fprintf('Błąd w ALGORYTMIE v2: %s\n', algErr.message);
        end
        % =========== ALGORYTM v2 END ==========
        
        % Aktualizacja wykresu
        drawnow;
    end
catch
    % Zamknięcie portu szeregowego w przypadku błędu lub zamknięcia okna
    closeSerialPort(gcf, serialPort);
end

% Funkcja resetująca zmienną ZBIOR
function resetZbior()
    % Resetowanie zmiennej ZBIOR
    global ZBIOR;
    ZBIOR = [0 0 0; 0 0 0; 0 0 0];
    
    global MAGNITUDE;
    MAGNITUDE = [1; 1; 1];

    global hLine5;
    global hLine6;
    clearpoints(hLine5);
    clearpoints(hLine6);
    
    % Aktualizacja danych na wykresie

    disp('Dane zostały zresetowane');
    % Aktualizacja wykresu
end

% Funkcja zamykająca port szeregowy i okno
function closeSerialPort(fig, serialPort)
    % Zamknięcie portu szeregowego
    if isvalid(serialPort)
        delete(serialPort);
        disp('Port szeregowy zamknięty.');
    end
    
    % Zamknięcie okna
    if isvalid(fig)
        delete(fig);
    end
end
