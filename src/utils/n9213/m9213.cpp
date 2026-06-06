#include "n9213/m9213.h"
QVector<double> m9213::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
