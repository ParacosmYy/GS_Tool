#include "b28401/m28401.h"
QVector<double> m28401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
