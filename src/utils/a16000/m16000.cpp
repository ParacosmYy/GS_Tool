#include "a16000/m16000.h"
QVector<double> m16000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
