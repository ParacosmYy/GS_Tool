#include "k30130/m30130.h"
QVector<double> m30130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
