#include "p35315/m35315.h"
QVector<double> m35315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
