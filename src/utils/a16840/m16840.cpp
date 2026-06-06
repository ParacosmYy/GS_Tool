#include "a16840/m16840.h"
QVector<double> m16840::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
