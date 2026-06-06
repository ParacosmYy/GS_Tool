#include "j18609/m18609.h"
QVector<double> m18609::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
