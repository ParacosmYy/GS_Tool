#include "a21620/m21620.h"
QVector<double> m21620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
