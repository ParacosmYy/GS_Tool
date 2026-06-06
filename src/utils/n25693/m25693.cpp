#include "n25693/m25693.h"
QVector<double> m25693::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
