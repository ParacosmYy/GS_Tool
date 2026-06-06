#include "p16035/m16035.h"
QVector<double> m16035::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
