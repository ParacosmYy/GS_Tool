#include "k30630/m30630.h"
QVector<double> m30630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
