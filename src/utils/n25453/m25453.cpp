#include "n25453/m25453.h"
QVector<double> m25453::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
