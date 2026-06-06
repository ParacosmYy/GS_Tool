#include "n9013/m9013.h"
QVector<double> m9013::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
