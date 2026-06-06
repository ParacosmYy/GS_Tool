#include "i26208/m26208.h"
QVector<double> m26208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
