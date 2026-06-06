#include "l9511/m9511.h"
QVector<double> m9511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
