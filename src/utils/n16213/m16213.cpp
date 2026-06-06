#include "n16213/m16213.h"
QVector<double> m16213::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
