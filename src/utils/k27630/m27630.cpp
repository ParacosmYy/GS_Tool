#include "k27630/m27630.h"
QVector<double> m27630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
