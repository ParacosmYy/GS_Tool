#include "k30310/m30310.h"
QVector<double> m30310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
