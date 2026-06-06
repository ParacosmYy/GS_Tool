#include "k18130/m18130.h"
QVector<double> m18130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
