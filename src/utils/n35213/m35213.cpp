#include "n35213/m35213.h"
QVector<double> m35213::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
