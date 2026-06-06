#include "s9698/m9698.h"
QVector<double> m9698::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
