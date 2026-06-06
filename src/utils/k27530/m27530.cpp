#include "k27530/m27530.h"
QVector<double> m27530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
