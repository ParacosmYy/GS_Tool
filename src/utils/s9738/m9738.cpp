#include "s9738/m9738.h"
QVector<double> m9738::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
