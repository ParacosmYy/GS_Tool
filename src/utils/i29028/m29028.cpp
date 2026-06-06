#include "i29028/m29028.h"
QVector<double> m29028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
