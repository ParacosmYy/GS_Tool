#include "n8733/m8733.h"
QVector<double> m8733::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
