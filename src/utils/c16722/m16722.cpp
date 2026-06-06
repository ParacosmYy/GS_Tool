#include "c16722/m16722.h"
QVector<double> m16722::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
