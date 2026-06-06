#include "b9421/m9421.h"
QVector<double> m9421::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
