#include "m9272/m9272.h"
QVector<double> m9272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
