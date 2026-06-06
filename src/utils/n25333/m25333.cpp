#include "n25333/m25333.h"
QVector<double> m25333::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
