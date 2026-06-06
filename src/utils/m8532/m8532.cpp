#include "m8532/m8532.h"
QVector<double> m8532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
