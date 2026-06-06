#include "l36511/m36511.h"
QVector<double> m36511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
