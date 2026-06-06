#include "f18325/m18325.h"
QVector<double> m18325::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
