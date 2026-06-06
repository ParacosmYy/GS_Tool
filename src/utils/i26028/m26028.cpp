#include "i26028/m26028.h"
QVector<double> m26028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
