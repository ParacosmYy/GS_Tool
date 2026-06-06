#include "i26488/m26488.h"
QVector<double> m26488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
