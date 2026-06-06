#include "m27532/m27532.h"
QVector<double> m27532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
