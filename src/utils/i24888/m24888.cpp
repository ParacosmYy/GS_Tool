#include "i24888/m24888.h"
QVector<double> m24888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
