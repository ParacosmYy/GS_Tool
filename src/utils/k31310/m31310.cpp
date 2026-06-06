#include "k31310/m31310.h"
QVector<double> m31310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
