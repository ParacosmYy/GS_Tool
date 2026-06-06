#include "m28092/m28092.h"
QVector<double> m28092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
