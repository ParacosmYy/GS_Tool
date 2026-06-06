#include "i26608/m26608.h"
QVector<double> m26608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
