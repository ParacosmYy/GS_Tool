#include "m18252/m18252.h"
QVector<double> m18252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
