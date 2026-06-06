#include "n9073/m9073.h"
QVector<double> m9073::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
