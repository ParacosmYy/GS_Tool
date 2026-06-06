#include "n8693/m8693.h"
QVector<double> m8693::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
