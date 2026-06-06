#include "p16115/m16115.h"
QVector<double> m16115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
