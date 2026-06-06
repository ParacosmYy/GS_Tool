#include "i26408/m26408.h"
QVector<double> m26408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
