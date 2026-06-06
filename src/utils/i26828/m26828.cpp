#include "i26828/m26828.h"
QVector<double> m26828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
