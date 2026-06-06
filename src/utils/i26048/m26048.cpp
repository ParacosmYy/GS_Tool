#include "i26048/m26048.h"
QVector<double> m26048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
