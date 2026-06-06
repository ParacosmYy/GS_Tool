#include "f31325/m31325.h"
QVector<double> m31325::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
