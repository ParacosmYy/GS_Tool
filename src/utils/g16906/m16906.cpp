#include "g16906/m16906.h"
QVector<double> m16906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
