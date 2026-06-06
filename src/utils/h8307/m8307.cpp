#include "h8307/m8307.h"
QVector<double> m8307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
