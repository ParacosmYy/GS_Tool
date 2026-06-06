#include "k27010/m27010.h"
QVector<double> m27010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
