#include "o8814/m8814.h"
QVector<double> m8814::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
