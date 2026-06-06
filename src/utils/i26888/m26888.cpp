#include "i26888/m26888.h"
QVector<double> m26888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
