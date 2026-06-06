#include "k18410/m18410.h"
QVector<double> m18410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
