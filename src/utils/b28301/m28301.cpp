#include "b28301/m28301.h"
QVector<double> m28301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
