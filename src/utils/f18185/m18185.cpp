#include "f18185/m18185.h"
QVector<double> m18185::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
