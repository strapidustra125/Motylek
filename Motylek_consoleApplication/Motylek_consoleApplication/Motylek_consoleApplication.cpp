
/* ====================================== Описание программы ===================================== */


/* ================================ Включение заголовочных файлов ================================ */

#include <iostream>             // Работа с консольными потоками ввода-вывода
#include <string>               // Работа со строками
#include <iomanip>              // Форматированный ввод-вывод
#include <fstream>              // Работа с файловыми потоками ввода-вывода

using namespace std;            // Определение пространства имен для библиотек



/* ========================================== Константы ========================================== */

#define STATE_NUMBER        30      // Количество состояний систем в сети (размерность матрицы)
#define TIME_STEP           0.001   // Шаг по времени для длительности каждого этапа
#define ITERATION_NUMBER    5       // Количество итераций посмотрения матрицы вероятностей

#define ERROR_MESSAGE   -1          // Константа ошибки


/* ==================================== Глобальные переменные ==================================== */

// Исходные данные

double lambda = 1;      // Основной поток требований
double mu1 = 3;         // Среднее число обслуживаемых требований первым прибором
double mu2 = 3;         // Среднее число обслуживаемых требований вторым прибором
double theta1 = 0.5;    // Вероятность попадания требования из очереди "lambda" в очередь системы S1
double theta2 = 0.5;    // Вероятность попадания требования из очереди "lambda" в очередь системы S2

double alpha = 0.01;    
double beta = 100;      
double tau = 1;         

// Длительности этапов

double step1_duration = 0;
double step2_duration = 0;
double step3_duration = 0;
double step4_duration = 0;

// Матрицы вероятностей

double P_main[STATE_NUMBER][STATE_NUMBER];      // Основная матрица вероятностей систем
                                                // Формируется на протяжении всей итерации

double P_temp[STATE_NUMBER][STATE_NUMBER];      // Временная матрица для пошаговой перезаписи
                                                // Формируется каждый этап итерации 
                                                // После заполнения перезаписывается в P_main

double P1[STATE_NUMBER][STATE_NUMBER];          // Матрица средних значений вероятностей на 1 шаге
                                                // Необходима для подсчета характеристик систем

double P2[STATE_NUMBER][STATE_NUMBER];          // Матрица средних значений вероятностей на 2 шаге
                                                // Необходима для подсчета характеристик систем

double P3[STATE_NUMBER][STATE_NUMBER];          // Матрица средних значений вероятностей на 3 шаге
                                                // Необходима для подсчета характеристик систем

double P4[STATE_NUMBER][STATE_NUMBER];          // Матрица средних значений вероятностей на 4 шаге
                                                // Необходима для подсчета характеристик систем

// Характеристики систем

double q_s1_1 = 0;  // Математическое ожидание числа требований в 1 системе на 1 этапе
double q_s1_2 = 0;  // Математическое ожидание числа требований в 1 системе на 2 этапе
double q_s1_3 = 0;  // Математическое ожидание числа требований в 1 системе на 3 этапе
double q_s1_4 = 0;  // Математическое ожидание числа требований в 1 системе на 4 этапе

double q_s1 = 0;    // Математическое ожидание числа требований в 1 системе

double q_s2_1 = 0;  // Математическое ожидание числа требований во 2 системе на 1 этапе
double q_s2_2 = 0;  // Математическое ожидание числа требований во 2 системе на 2 этапе
double q_s2_3 = 0;  // Математическое ожидание числа требований во 2 системе на 3 этапе
double q_s2_4 = 0;  // Математическое ожидание числа требований во 2 системе на 4 этапе

double q_s2 = 0;    // Математическое ожидание числа требований во 2 системе

double lambda1 = 0; // 
double lambda2 = 0; // 

double u1 = 0;      // Математическое ожидание длительности требований в 1 системе
double u2 = 0;      // Математическое ожидание длительности требований во 2 системе

// Имена файлов

string file_characteristics = "characteristics.txt";

/* ----------------------------------------------------------------------------------------------- */




/* ============================ Функции чтения параметров из консоли ============================= */

// Перевод строки в положительное вещественное число
// Возвращает: положительное вещественное число при успехе или -1 при ошибке
double stringToARealPositiveNumber(string inputString)
{
    string resultString = "";   // Строка, хранящая результирующее число
    double resultNumber = 0;    // Результирующее число для возврата из функции
    int pointsCounter = 0;      // Счетчик точек в строке

    // Проходим по всем элементам строки и формируем "resultString"
    for (int i = 0; i < inputString.length(); i++)
    {
        // Допустимые символы: точка и цифры 0-9 
        if (((int)inputString[i] == 46) || ((int)inputString[i] >= 48 && (int)inputString[i] <= 57))
        {
            // Обработка встретившейся в числе точки '.'
            if ((int)inputString[i] == 46)
            {
                // Ошибка, если это не первая точка
                if (++pointsCounter > 1) return ERROR_MESSAGE;    

                // Ошибка, если строка начинается с точки
                else if (i == 0) return ERROR_MESSAGE;           

                // Ошибка, если строка заканчивается точкой
                else if (i == inputString.length() - 1) return ERROR_MESSAGE;  

                // Иначе меняем точку на запятую, иначе не работает функция "stod"
                else resultString += ',';
            }
            else resultString += inputString[i];    // Дописываем символ, если это просто цифра
        }
        else return ERROR_MESSAGE;  // Ошибка если это не точка или цифра 0-9
    }

    // Переводим строку "resultString" в число "resultNumber"
    resultNumber = stod(resultString);

    return resultNumber;    // Возвращаем положительное вещественное число
}


/* ----------------------------------------------------------------------------------------------- */

// Чтение заданного пользователем параметра из консоли
double readParametr(string parametr)
{
    string parametrValueString = "";    // Значение считываемого параметра
    double resultNumber = 0;

    cout << "\tВведите значение параметра \"" << parametr << "\": ";

    // Пока не будет введено верное значение параметра
    while (1)
    {
        cin >> parametrValueString;     // Чтение очередного значения параметра
        cout << endl;                   // Пустая строка для повышения читабельности

        resultNumber = stringToARealPositiveNumber(parametrValueString);

        if (resultNumber != ERROR_MESSAGE) return resultNumber;
        else
        {
            cout << "\tНеверный формат ввода, попробуйте еще раз... " << endl;
            cout << "\tВведите \"" << parametr << "\": ";
        }
    }

    return 1;
}


/* ----------------------------------------------------------------------------------------------- */

//
bool askToReadParam()
{
    string choice = "";

    cout << "1 - Ввести пользовательские параметры" << endl << "2 - Использовать по умолчанию" << endl;

    cout << "Выберите метод инициализации (1 или 2): ";

    cin >> choice;

    while (1)
    {
        if (choice == "1") return true;
        else if (choice == "2") return false;
        else
        {
            cout << "Неверный формат ввода. Выберите 1 или 2: ";
            cin >> choice;
        }
    }
}


/* =================================== Функции инициализации ===================================== */

// Инициализация параметров систем пользователем
void initSystemsParametrs()
{
    cout << endl << "Введите параметры системы (положительные вещественные числа)..." << endl << endl;

    lambda  = readParametr("lambda");
    mu1     = readParametr("mu1");
    mu2     = readParametr("mu2");
    theta1  = readParametr("theta1");
    theta2  = readParametr("theta2");
    alpha   = readParametr("alpha");
    beta    = readParametr("beta");
    tau     = readParametr("tau");
}





/* ================================ Функции работы с матрицами =================================== */

// Стартовая инициализация матрицы размера STATE_NUMBER
void startMaxtixInit(double matrix[STATE_NUMBER][STATE_NUMBER])
{
    // Цикл по строкам
    for (int i = 0; i < STATE_NUMBER; i++)
    {
        // Цикл по столбцам
        for (int j = 0; j < STATE_NUMBER; j++)
        {
            matrix[i][j] = 0;
        }
    }

    matrix[0][0] = 1;    // Первый элемент
}


/* ----------------------------------------------------------------------------------------------- */

// Перезапись матрицы P_temp в P_main после завершения шага
void updateMainMatrixFromPTemp()
{
    for (int i = 0; i < STATE_NUMBER; i++)
    {
        for (int j = 0; j < STATE_NUMBER; j++)
        {
            P_main[i][j] = P_temp[i][j];
        }
    }
}


/* ----------------------------------------------------------------------------------------------- */



// 
double checkPMain()
{
    double sumProbability = 0;

    for (int i = 0; i < STATE_NUMBER; i++)
    {
        for (int j = 0; j < STATE_NUMBER; j++)
        {
            sumProbability += P_main[i][j];
        }
    }

    return sumProbability;
}

void initAdditionMatrix()
{
    for (int i = 0; i < STATE_NUMBER; i++)
    {
        // Цикл по столбцам
        for (int j = 0; j < STATE_NUMBER; j++)
        {
            P1[i][j] = 0;
            P2[i][j] = 0;
            P3[i][j] = 0;
            P4[i][j] = 0;
        }
    }
}


/* ====================================== Функции отрисовки ===================================== */

// Отрисовать прочитанные параметры систем
void printConsoleParametrs()
{
    cout << endl << "Прочитанные параметры систем:" << endl << endl;

    cout << "\tlambda = " << lambda << endl;
    cout << "\tmu1 = " << mu1 << endl;
    cout << "\tmu2 = " << mu2 << endl;
    cout << "\ttheta1 = " << theta1 << endl;
    cout << "\ttheta2 = " << theta2 << endl;
    cout << "\talpha = " << alpha << endl;
    cout << "\tbeta = " << beta << endl;
    cout << "\ttau = " << tau << endl << endl;
}

// Отрисовка матрицы вероятностей в консоль
void printPMain()
{
    cout << endl << "Матрица вероятностей:" << endl << endl;

    // Цикл по строкам
    for (int i = 0; i < STATE_NUMBER; i++)
    {
        cout << "  ";   // Отступ перед очередной строкой матрицы

        // Цикл по столбцам
        for (int j = 0; j < STATE_NUMBER; j++)
        {
            // Вывод элементов матрицы
            // setw - ширина выводимого поля 
            // setprecision - точность числе с плавающей точкой 
            cout << setw(9) << setprecision(3) << P_main[i][j] << " ";
        }
        cout << endl;
    }

    cout << endl;
}



// Отрисовка матрицы вероятностей в консоль
void consolePrintMatrix(double matrix[STATE_NUMBER][STATE_NUMBER], string matrixName)
{
    cout << endl << "Матрица вероятностей \"" << matrixName << "\":" << endl << endl;

    // Цикл по строкам
    for (int i = 0; i < STATE_NUMBER; i++)
    {
        cout << "  ";   // Отступ перед очередной строкой матрицы

        // Цикл по столбцам
        for (int j = 0; j < STATE_NUMBER; j++)
        {
            // Вывод элементов матрицы
            // setw - ширина выводимого поля 
            // setprecision - точность числе с плавающей точкой 
            cout << setw(9) << setprecision(3) << matrix[i][j] << " ";
        }
        cout << endl;
    }

    cout << endl;
}



void filePrintMatrix(double matrix[STATE_NUMBER][STATE_NUMBER], string matrixName, ofstream& outStream)
{
    outStream << endl << "Матрица вероятностей \"" << matrixName << "\":" << endl << endl;

    // Цикл по строкам
    for (int i = 0; i < STATE_NUMBER; i++)
    {
        outStream << "  ";   // Отступ перед очередной строкой матрицы

        // Цикл по столбцам
        for (int j = 0; j < STATE_NUMBER; j++)
        {
            // Вывод элементов матрицы
            // setw - ширина выводимого поля 
            // setprecision - точность числе с плавающей точкой 
            outStream << setw(9) << setprecision(3) << matrix[i][j] << " ";
        }
        outStream << endl;
    }

    cout << endl;
}


void filePrintParams(ofstream& outStream)
{
    outStream << "Параметры сети:" << endl << endl;

    outStream << "\tОсновной поток требований." << endl;
    outStream << "\t\t\"lambda\" = " << lambda << endl << endl;

    outStream << "\tСреднее число обслуживаемых требований первым прибором." << endl;
    outStream << "\t\t\"mu1\" = " << mu1 << endl << endl;

    outStream << "\tСреднее число обслуживаемых требований вторым прибором." << endl;
    outStream << "\t\t\"mu2\" = " << mu2 << endl << endl;

    outStream << "\tВероятность попадания требования из очереди \"lambda\" в очередь системы S1." << endl;
    outStream << "\t\t\"theta1\" = " << theta1 << endl << endl;

    outStream << "\tВероятность попадания требования из очереди \"lambda\" в очередь системы S2."<< endl << endl;
    outStream << "\t\t\"theta2\" = " << theta2 << endl << endl;


    outStream << "Длительности этапов:" << endl << endl;

    outStream << "\tДлительность первого этапа." << endl;
    outStream << "\t\t\"1 / alpha\" = " << 1 / alpha << endl << endl;

    outStream << "\tДлительность второго этапа." << endl;
    outStream << "\t\t\"tau\" = " << tau << endl << endl;

    outStream << "\tДлительность третьего этапа." << endl;
    outStream << "\t\t\"1 / beta\" = " << 1 / beta << endl << endl;

    outStream << "\tДлительность четвертого этапа." << endl << endl;
    outStream << "\t\t\"tau\" = " << tau << endl << endl;
}



void filePrintCharacteristics(ofstream& outStream)
{
    outStream << "Характеристики сети:" << endl << endl;

    outStream << "\tМатематическое ожидание числа требований в 1 системе на 1 этапе." << endl;
    outStream << "\t\t\"q_s1_1\" = " << q_s1_1 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований в 1 системе на 2 этапе." << endl;
    outStream << "\t\t\"q_s1_2\" = " << q_s1_2 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований в 1 системе на 3 этапе." << endl;
    outStream << "\t\t\"q_s1_3\" = " << q_s1_3 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований в 1 системе на 4 этапе." << endl;
    outStream << "\t\t\"q_s1_4\" = " << q_s1_4 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований в 1 системе." << endl;
    outStream << "\t\t\"q_s1\" = " << q_s1 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований во 2 системе на 1 этапе." << endl;
    outStream << "\t\t\"q_s2_1\" = " << q_s2_1 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований во 2 системе на 2 этапе." << endl;
    outStream << "\t\t\"q_s2_2\" = " << q_s2_2 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований во 2 системе на 3 этапе." << endl;
    outStream << "\t\t\"q_s2_3\" = " << q_s2_3 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований во 2 системе на 4 этапе." << endl;
    outStream << "\t\t\"q_s2_4\" = " << q_s2_4 << endl << endl;

    outStream << "\tМатематическое ожидание числа требований во 2 системе." << endl;
    outStream << "\t\t\"q_s2\" = " << q_s2 << endl << endl;


    outStream << "\t??????." << endl;
    outStream << "\t\t\"lambda1\" = " << lambda1 << endl << endl;

    outStream << "\t??????." << endl;
    outStream << "\t\t\"lambda2\" = " << lambda2 << endl << endl;


    outStream << "\tМатематическое ожидание длительности требований в 1 системе." << endl;
    outStream << "\t\t\"u1\" = " << u1 << endl << endl;

    outStream << "\tМатематическое ожидание длительности требований во 2 системе." << endl; 
    outStream << "\t\t\"u2\" = " << u2 << endl << endl;
}






/* ======================== Функции работы с дифференциальными уравнениями ======================= */

// Параметр "Эпсилон"
int epsilon(int state)
{
    if (state > 0) return 1;        // 1, если в очереди есть требования
    else if (state == 0) return 0;  // 0, если нет
    else return ERROR_MESSAGE;      // -1 - ошибка
}



/* ----------------------------------------------------------------------------------------------- */

                    /* ======== Подсчет функции на каждом этапе ======== */

// 
double step1_function(int i, int j)
{
    return -(lambda + epsilon(i) * mu1 + epsilon(j) * mu2) * P_main[i][j] +
        lambda * (epsilon(i) * theta1 * P_main[i - 1][j] + epsilon(j) * theta2 * P_main[i][j - 1]) +
        mu1 * P_main[i + 1][j] + mu2 * P_main[i][j + 1];
}

/* ----------------------------------------------------------------------------------------------- */

// 
double step2_function(int i, int j)
{
    return -(lambda + epsilon(i) * mu1) * P_main[i][j] +
        lambda * (epsilon(i) * theta1 * P_main[i - 1][j] + epsilon(j) * theta2 * P_main[i][j - 1]) +
        mu1 * P_main[i + 1][j];
}


/* ----------------------------------------------------------------------------------------------- */

// 
double step3_function(int i, int j)
{
    return -(lambda + epsilon(i) * mu1) * P_main[i][j] + lambda * epsilon(i) * P_main[i - 1][j] +
        mu1 * P_main[i + 1][j];
}


/* ----------------------------------------------------------------------------------------------- */

// 
double step4_function(int i, int j)
{
    return -(lambda + epsilon(i) * mu1 + epsilon(j) * mu2) * P_main[i][j] +
        lambda * epsilon(i) * P_main[i - 1][j] + mu1 * P_main[i + 1][j] + mu2 * P_main[i][j + 1];
}

/* ----------------------------------------------------------------------------------------------- */

                    /* ======== Обработка каждого этапа ======== */

// Итерационное построение матрицы вероятностей на первом этапе
void step1_updateMatrix(int iter)
{
    double currentTime = 0;

    while (currentTime <= step1_duration)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P_temp[i][j] = P_main[i][j] + TIME_STEP * step1_function(i, j);

                if (iter == ITERATION_NUMBER - 1) P1[i][j] += P_temp[i][j];
            }
        }

        updateMainMatrixFromPTemp();
        currentTime += TIME_STEP;
    }

    if (iter == ITERATION_NUMBER - 1)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P1[i][j] /= (step1_duration / TIME_STEP);
            }
        }
    }
}


/* ----------------------------------------------------------------------------------------------- */

// Итерационное построение матрицы вероятностей на втором этапе
void step2_updateMatrix(int iter)
{
    double currentTime = 0;

    while (currentTime <= step2_duration)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P_temp[i][j] = P_main[i][j] + TIME_STEP * step2_function(i, j);

                if (iter == ITERATION_NUMBER - 1) P2[i][j] += P_temp[i][j];
            }
        }

        updateMainMatrixFromPTemp();
        currentTime += TIME_STEP;
    }

    if (iter == ITERATION_NUMBER - 1)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P2[i][j] /= (step2_duration / TIME_STEP);
            }
        }
    }
}


/* ----------------------------------------------------------------------------------------------- */

// Итерационное построение матрицы вероятностей на третьем этапе
void step3_updateMatrix(int iter)
{
    double currentTime = 0;

    while (currentTime <= step3_duration)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P_temp[i][j] = P_main[i][j] + TIME_STEP * step3_function(i, j);

                if (iter == ITERATION_NUMBER - 1) P3[i][j] += P_temp[i][j];
            }
        }

        updateMainMatrixFromPTemp();
        currentTime += TIME_STEP;
    }

    if (iter == ITERATION_NUMBER - 1)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P3[i][j] /= (step3_duration / TIME_STEP);
            }
        }
    }
}


/* ----------------------------------------------------------------------------------------------- */

// Итерационное построение матрицы вероятностей на четвертом этапе
void step4_updateMatrix(int iter)
{
    double currentTime = 0;
    double sumProbability = 0;

    while (currentTime <= step4_duration)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P_temp[i][j] = P_main[i][j] + TIME_STEP * step4_function(i, j);

                if (iter == ITERATION_NUMBER - 1) P4[i][j] += P_temp[i][j];
            }
        }

        updateMainMatrixFromPTemp();
        currentTime += TIME_STEP;
    }

    if (iter == ITERATION_NUMBER - 1)
    {
        for (int i = 0; i < STATE_NUMBER - 1; i++)
        {
            for (int j = 0; j < STATE_NUMBER - 1; j++)
            {
                P4[i][j] /= (step4_duration / TIME_STEP);
            }
        }
    }
    

}


/* ----------------------------------------------------------------------------------------------- */

// 
void countCharacteristics()
{
    double sumDuration = step1_duration + step2_duration + step3_duration + step4_duration;

    for (int i = 0; i < STATE_NUMBER; i++) q_s1_1 += i * P1[0][i];
    for (int i = 0; i < STATE_NUMBER; i++) q_s1_2 += i * P2[0][i];
    for (int i = 0; i < STATE_NUMBER; i++) q_s1_3 += i * P3[0][i];
    for (int i = 0; i < STATE_NUMBER; i++) q_s1_4 += i * P4[0][i];

    q_s1 =  q_s1_1 * (step1_duration / sumDuration) + q_s1_2 * (step2_duration / sumDuration) + 
            q_s1_3 * (step3_duration / sumDuration) + q_s1_4 * (step4_duration / sumDuration);

    for (int i = 0; i < STATE_NUMBER; i++) q_s2_1 += i * P1[i][0];
    for (int i = 0; i < STATE_NUMBER; i++) q_s2_2 += i * P2[i][0];
    for (int i = 0; i < STATE_NUMBER; i++) q_s2_3 += i * P3[i][0];
    for (int i = 0; i < STATE_NUMBER; i++) q_s2_4 += i * P4[i][0];

    q_s2 = q_s2_1 * (step1_duration / sumDuration) + q_s2_2 * (step2_duration / sumDuration) +
        q_s2_3 * (step3_duration / sumDuration) + q_s2_4 * (step4_duration / sumDuration);

    lambda1 = lambda * theta1 * ((step1_duration + step2_duration) / sumDuration) + 
        lambda * ((step3_duration + step4_duration) / sumDuration);

    lambda2 = lambda * theta2 * ((step1_duration + step2_duration) / sumDuration);

    u1 = q_s1 / lambda1;
    u2 = q_s2 / lambda2;
}

/* ----------------------------------------------------------------------------------------------- */












/* ----------------------------------------------------------------------------------------------- */

/* ================================= Основная функция приложения ================================= */

int main()
{
    ofstream out(file_characteristics);

    setlocale(LC_ALL, "Russian");

    cout << endl << "< ==== Программа моделирования СМО ==== >" << endl << endl;

    startMaxtixInit(P_main);
    startMaxtixInit(P_temp);
    initAdditionMatrix();

    if (askToReadParam())
    {
        initSystemsParametrs();     // Инициализация параметров систем
    }

    printConsoleParametrs();
    
    step1_duration = 1 / alpha;
    step2_duration = tau;
    step3_duration = 1 / beta;
    step4_duration = tau;

    cout << "Начало итерационной процедуры построения метрицы вероятностей..." << endl << endl;

    for(int iter = 0; iter < ITERATION_NUMBER; iter++)
    {
        step1_updateMatrix(iter);
        step2_updateMatrix(iter);
        step3_updateMatrix(iter);
        step4_updateMatrix(iter);

        cout << "\tПосчитано итераций: " << iter << endl;
    }

    cout << endl << "Процедура завершена!" << endl << endl;

    countCharacteristics();

    filePrintParams(out);
    filePrintCharacteristics(out);

    filePrintMatrix(P_main, "P", out);

    return 0;
}


/*

4 временные матрицы для каждого шага, считают средние значения вероятностей, но только для последней итерации

мат ожидание числа требований q1-1(2-3-4) с чертой (для первой системы)

4 мат ожидания для каждого шага на последней итерации для первой и второй системы
первая системы - это 1 строка, вторая - 1 столбец


стеднее мат ожидание для 4-х этапов для каждой системы - ку 1 и 2 с чертой



лямбда 2 штуки для каждой системы

u 

u1 и u2 



В приложение весь код программы без комментариев

пдп - 20 стр

вкр - 50 стр

скрины кода? куски кода? латех? можно 12 шрифт

все в латехе

выходной файл в отчет



















*/