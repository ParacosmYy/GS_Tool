#include "m13532/m13532.h"
QVector<double> m13532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
