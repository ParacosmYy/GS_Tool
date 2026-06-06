#include "a16440/m16440.h"
QVector<double> m16440::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
