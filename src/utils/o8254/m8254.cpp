#include "o8254/m8254.h"
QVector<double> m8254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
