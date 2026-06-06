#include "k27310/m27310.h"
QVector<double> m27310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
