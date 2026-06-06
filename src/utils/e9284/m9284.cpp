#include "e9284/m9284.h"
QVector<double> m9284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
