#include "a30400/m30400.h"
QVector<double> m30400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
